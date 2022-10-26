function gitInfo=getCurrentVersion()

gitInfo=[];
gitInfo.version = 0;
if ~exist('.git','file') || ~exist('.git/HEAD','file')
    %Git is not present
    return
end

text=fileread('.git/HEAD');
parsed=textscan(text,'%s');

if length(text) == 41
    gitInfo.hash = text;
    tag = getGitTag(text);
    if (isempty(tag))
        gitInfo.version = gitInfo.hash;
    else
        gitInfo.version = tag;
    end
    return
end

if ~strcmp(parsed{1}{1},'ref:') || ~length(parsed{1})>1
    %the HEAD is not in the expected format.
    %give up
    return
end

path=parsed{1}{2};
[pathstr, branchName, ext] = fileparts(path);


%Read in SHA1
SHA1text = fileread(fullfile(['.git/' pathstr],[branchName ext]));
SHA1 = textscan(SHA1text,'%s');
gitInfo.hash = SHA1{1}{1};

%save branchname
gitInfo.version = strcat(branchName, '/', extractBefore(gitInfo.hash, 7));

end

function tagName=getGitTag(sha1)
    tagFiles = dir('.git/refs/tags/*');
    for idx=length(tagFiles):-1:1        
        if (tagFiles(idx).isdir == 0)            
            tagPath = strcat(tagFiles(idx).folder, '/', tagFiles(idx).name);            
            tagSha1=fileread(tagPath);
            if (strcmp(sha1,tagSha1))
                tagName = tagFiles(idx).name;
                return;
            end        
        end
    end
    tagName = '';
end